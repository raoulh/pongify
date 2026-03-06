package main

import (
	"bufio"
	"bytes"
	"crypto/rand"
	_ "embed"
	"encoding/hex"
	"encoding/json"
	"fmt"
	"io"
	"mime/multipart"
	"net/http"
	"net/textproto"
	"os"
	"strings"
	"time"
)

//go:embed worker.js
var workerJS string

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

const (
	cfBaseURL  = "https://api.cloudflare.com/client/v4"
	workerName = "pongify-live"
	kvTitle    = "PONGIFY_TOURNAMENTS"

	charCheck   = "\u2714"
	charCross   = "\u2718"
	charArrow   = "\u2192"
	charSpinner = "\u25cb"
)

// ---------------------------------------------------------------------------
// Cloudflare API helpers
// ---------------------------------------------------------------------------

var httpClient = &http.Client{Timeout: 30 * time.Second}

// cfResponse is the generic envelope for Cloudflare API responses.
type cfResponse struct {
	Success bool              `json:"success"`
	Errors  []cfError         `json:"errors"`
	Result  json.RawMessage   `json:"result"`
	ResultInfo *cfResultInfo  `json:"result_info,omitempty"`
}

type cfError struct {
	Code    int    `json:"code"`
	Message string `json:"message"`
}

type cfResultInfo struct {
	TotalCount int `json:"total_count"`
}

// cfAPI performs an HTTP request to the Cloudflare API, parses the response
// envelope, and returns the parsed response. Returns an error if the request
// fails or if the Cloudflare API returns success=false.
func cfAPI(method, url, token string, body io.Reader, contentType string) (*cfResponse, error) {
	req, err := http.NewRequest(method, url, body)
	if err != nil {
		return nil, fmt.Errorf("creating request: %w", err)
	}
	req.Header.Set("Authorization", "Bearer "+token)
	if contentType != "" {
		req.Header.Set("Content-Type", contentType)
	}

	resp, err := httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("HTTP request failed: %w", err)
	}
	defer resp.Body.Close()

	respBody, err := io.ReadAll(resp.Body)
	if err != nil {
		return nil, fmt.Errorf("reading response: %w", err)
	}

	var cfResp cfResponse
	if err := json.Unmarshal(respBody, &cfResp); err != nil {
		return nil, fmt.Errorf("parsing response (status %d): %s", resp.StatusCode, string(respBody))
	}

	if !cfResp.Success {
		msgs := make([]string, len(cfResp.Errors))
		for i, e := range cfResp.Errors {
			msgs[i] = fmt.Sprintf("[%d] %s", e.Code, e.Message)
		}
		return &cfResp, fmt.Errorf("API error: %s", strings.Join(msgs, "; "))
	}

	return &cfResp, nil
}

// cfAPIRaw is like cfAPI but for endpoints whose body we handle with
// a custom content-type (multipart). It feeds the body bytes directly.
func cfAPIRaw(method, url, token string, body []byte, contentType string) (*cfResponse, error) {
	return cfAPI(method, url, token, bytes.NewReader(body), contentType)
}

// ---------------------------------------------------------------------------
// Utility
// ---------------------------------------------------------------------------

func prompt(label string) string {
	fmt.Print(label)
	scanner := bufio.NewScanner(os.Stdin)
	scanner.Scan()
	return strings.TrimSpace(scanner.Text())
}

func generateHexSecret(nBytes int) (string, error) {
	b := make([]byte, nBytes)
	if _, err := rand.Read(b); err != nil {
		return "", err
	}
	return hex.EncodeToString(b), nil
}

func fatal(msg string) {
	fmt.Fprintf(os.Stderr, "\n%s %s\n", charCross, msg)
	os.Exit(1)
}

// ---------------------------------------------------------------------------
// Steps
// ---------------------------------------------------------------------------

// step1: get the Cloudflare API token
func getToken() string {
	// Check for --token argument
	for i, arg := range os.Args[1:] {
		if arg == "--token" && i+1 < len(os.Args[1:])-0 {
			// os.Args[1:] is 0-indexed, so the value is at i+1 in that slice
			if i+2 <= len(os.Args[1:]) {
				return strings.TrimSpace(os.Args[i+2])
			}
		}
	}

	fmt.Println("=== Pongify Live - Setup ===")
	fmt.Println()
	fmt.Println("Ce programme configure votre compte Cloudflare pour Pongify Live.")
	fmt.Println()
	fmt.Println("1. Allez sur https://dash.cloudflare.com/profile/api-tokens")
	fmt.Println("2. Créez un token avec les permissions:")
	fmt.Println("   - Account > Workers Scripts > Edit")
	fmt.Println("   - Account > Workers KV Storage > Edit")
	fmt.Println("3. Collez le token ci-dessous:")
	fmt.Println()
	token := prompt("Token API: ")
	if token == "" {
		fatal("Token vide, abandon.")
	}
	return token
}

// step2+3: verify token by listing accounts (no extra permission needed)
func getAccountID(token string) string {
	fmt.Printf("\n%s Vérification du token et récupération du compte...\n", charSpinner)
	resp, err := cfAPI("GET", cfBaseURL+"/accounts?page=1&per_page=5", token, nil, "")
	if err != nil {
		fatal(fmt.Sprintf("Token invalide ou impossible de récupérer les comptes: %v", err))
	}

	var accounts []struct {
		ID   string `json:"id"`
		Name string `json:"name"`
	}
	if err := json.Unmarshal(resp.Result, &accounts); err != nil {
		fatal(fmt.Sprintf("Impossible de parser les comptes: %v", err))
	}
	if len(accounts) == 0 {
		fatal("Aucun compte Cloudflare trouvé pour ce token.")
	}

	fmt.Printf("%s Compte: %s (%s)\n", charCheck, accounts[0].Name, accounts[0].ID)
	return accounts[0].ID
}

// step4: create or get KV namespace
func ensureKVNamespace(token, accountID string) string {
	fmt.Printf("\n%s Création du namespace KV...\n", charSpinner)

	body := fmt.Sprintf(`{"title":"%s"}`, kvTitle)
	resp, err := cfAPI("POST",
		fmt.Sprintf("%s/accounts/%s/storage/kv/namespaces", cfBaseURL, accountID),
		token, strings.NewReader(body), "application/json")

	if err == nil {
		// Created successfully
		var ns struct {
			ID string `json:"id"`
		}
		if err := json.Unmarshal(resp.Result, &ns); err != nil {
			fatal(fmt.Sprintf("Impossible de parser le namespace: %v", err))
		}
		fmt.Printf("%s Namespace KV créé: %s\n", charCheck, ns.ID)
		return ns.ID
	}

	// Already exists — list and find by title
	fmt.Printf("  %s Namespace existe déjà, recherche...\n", charArrow)
	listResp, listErr := cfAPI("GET",
		fmt.Sprintf("%s/accounts/%s/storage/kv/namespaces?per_page=100", cfBaseURL, accountID),
		token, nil, "")
	if listErr != nil {
		fatal(fmt.Sprintf("Impossible de lister les namespaces: %v", listErr))
	}

	var namespaces []struct {
		ID    string `json:"id"`
		Title string `json:"title"`
	}
	if err := json.Unmarshal(listResp.Result, &namespaces); err != nil {
		fatal(fmt.Sprintf("Impossible de parser les namespaces: %v", err))
	}

	for _, ns := range namespaces {
		if ns.Title == kvTitle {
			fmt.Printf("%s Namespace KV existant: %s\n", charCheck, ns.ID)
			return ns.ID
		}
	}

	fatal("Namespace KV introuvable après création échouée. Vérifiez les permissions du token.")
	return "" // unreachable
}

// step5: generate admin secret
func genAdminSecret() string {
	secret, err := generateHexSecret(32)
	if err != nil {
		fatal(fmt.Sprintf("Impossible de générer le secret: %v", err))
	}
	return secret
}

// step6: deploy the worker
func deployWorker(token, accountID, namespaceID string) {
	fmt.Printf("\n%s Déploiement du Worker...\n", charSpinner)

	// Build multipart body
	var buf bytes.Buffer
	writer := multipart.NewWriter(&buf)

	// Part 1: worker.js (ES module)
	jsHeader := make(textproto.MIMEHeader)
	jsHeader.Set("Content-Disposition", `form-data; name="worker.js"; filename="worker.js"`)
	jsHeader.Set("Content-Type", "application/javascript+module")
	jsPart, err := writer.CreatePart(jsHeader)
	if err != nil {
		fatal(fmt.Sprintf("Erreur création part JS: %v", err))
	}
	if _, err := jsPart.Write([]byte(workerJS)); err != nil {
		fatal(fmt.Sprintf("Erreur écriture part JS: %v", err))
	}

	// Part 2: metadata
	metadata := fmt.Sprintf(`{
  "main_module": "worker.js",
  "bindings": [
    {
      "type": "kv_namespace",
      "name": "KV",
      "namespace_id": "%s"
    }
  ],
  "compatibility_date": "2024-01-01"
}`, namespaceID)

	metaHeader := make(textproto.MIMEHeader)
	metaHeader.Set("Content-Disposition", `form-data; name="metadata"`)
	metaHeader.Set("Content-Type", "application/json")
	metaPart, err := writer.CreatePart(metaHeader)
	if err != nil {
		fatal(fmt.Sprintf("Erreur création part metadata: %v", err))
	}
	if _, err := metaPart.Write([]byte(metadata)); err != nil {
		fatal(fmt.Sprintf("Erreur écriture part metadata: %v", err))
	}

	writer.Close()

	url := fmt.Sprintf("%s/accounts/%s/workers/scripts/%s", cfBaseURL, accountID, workerName)
	_, err = cfAPIRaw("PUT", url, token, buf.Bytes(), writer.FormDataContentType())
	if err != nil {
		fatal(fmt.Sprintf("Échec du déploiement: %v", err))
	}

	fmt.Printf("%s Worker déployé\n", charCheck)
}

// step7: set the ADMIN_SECRET
func setAdminSecret(token, accountID, adminSecret string) {
	fmt.Printf("\n%s Configuration du secret ADMIN_SECRET...\n", charSpinner)

	body := fmt.Sprintf(`{"name":"ADMIN_SECRET","text":"%s","type":"secret_text"}`, adminSecret)
	url := fmt.Sprintf("%s/accounts/%s/workers/scripts/%s/secrets", cfBaseURL, accountID, workerName)
	_, err := cfAPI("PUT", url, token, strings.NewReader(body), "application/json")
	if err != nil {
		fatal(fmt.Sprintf("Échec de la configuration du secret: %v", err))
	}

	fmt.Printf("%s Secret configuré\n", charCheck)
}

// step8: ensure workers.dev subdomain is enabled (account-level + script-level)
func enableSubdomain(token, accountID string) {
	fmt.Printf("\n%s Activation du sous-domaine workers.dev...\n", charSpinner)

	// Account-level: enable workers.dev subdomain
	url := fmt.Sprintf("%s/accounts/%s/workers/subdomain", cfBaseURL, accountID)
	_, err := cfAPI("POST", url, token, strings.NewReader(`{"enabled":true}`), "application/json")
	if err != nil {
		fmt.Printf("  %s Sous-domaine account déjà activé (ignoré)\n", charArrow)
	} else {
		fmt.Printf("%s Sous-domaine account activé\n", charCheck)
	}

	// Script-level: enable workers.dev route for this specific worker
	scriptURL := fmt.Sprintf("%s/accounts/%s/workers/scripts/%s/subdomain", cfBaseURL, accountID, workerName)
	_, err = cfAPI("POST", scriptURL, token, strings.NewReader(`{"enabled":true}`), "application/json")
	if err != nil {
		fmt.Printf("  %s Route workers.dev du script déjà activée (ignoré)\n", charArrow)
	} else {
		fmt.Printf("%s Route workers.dev activée pour %s\n", charCheck, workerName)
	}
}

// step9: get the worker URL
func getWorkerURL(token, accountID string) string {
	fmt.Printf("\n%s Récupération de l'URL du Worker...\n", charSpinner)

	url := fmt.Sprintf("%s/accounts/%s/workers/subdomain", cfBaseURL, accountID)
	resp, err := cfAPI("GET", url, token, nil, "")
	if err != nil {
		fatal(fmt.Sprintf("Impossible de récupérer le sous-domaine: %v", err))
	}

	var sub struct {
		Subdomain string `json:"subdomain"`
	}
	if err := json.Unmarshal(resp.Result, &sub); err != nil {
		fatal(fmt.Sprintf("Impossible de parser le sous-domaine: %v", err))
	}

	workerURL := fmt.Sprintf("https://%s.%s.workers.dev", workerName, sub.Subdomain)
	fmt.Printf("%s URL: %s\n", charCheck, workerURL)
	return workerURL
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------

func main() {
	// Step 1: get token
	token := getToken()

	// Step 2+3: verify token + get account ID
	accountID := getAccountID(token)

	// Step 4: KV namespace
	namespaceID := ensureKVNamespace(token, accountID)

	// Step 5: admin secret
	adminSecret := genAdminSecret()

	// Step 6: deploy worker
	deployWorker(token, accountID, namespaceID)

	// Step 7: set secret
	setAdminSecret(token, accountID, adminSecret)

	// Step 8: enable workers.dev subdomain
	enableSubdomain(token, accountID)

	// Step 9: get URL
	workerURL := getWorkerURL(token, accountID)

	// Step 10: display results
	fmt.Println()
	fmt.Println("============================================")
	fmt.Printf("%s Worker déployé avec succès !\n", charCheck)
	fmt.Println("============================================")
	fmt.Println()
	fmt.Printf("URL du Worker:  %s\n", workerURL)
	fmt.Printf("Admin Secret:   %s\n", adminSecret)
	fmt.Println()
	fmt.Println("Collez ces informations dans Pongify:")
	fmt.Printf("Menu %s Outils %s Configuration publication web\n", charArrow, charArrow)
	fmt.Println()

	// Machine-readable JSON on the last line
	result := map[string]string{
		"worker_url":   workerURL,
		"admin_secret": adminSecret,
	}
	jsonBytes, _ := json.Marshal(result)
	fmt.Println(string(jsonBytes))
}
