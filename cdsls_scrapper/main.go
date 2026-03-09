package main

import (
	"encoding/json"
	"fmt"
	"regexp"
	"strings"

	"github.com/gocolly/colly/v2"
)

const (
	BASE_URL        = "https://cdsls.fr/"
	PLAYER_LIST_URL = "https://cdsls.fr/index.php?page=joueurs&abc=%s&jrs=1"
)

const (
	CharStar     = "\u2737"
	CharAbort    = "\u2718"
	CharCheck    = "\u2714"
	CharWarning  = "\u26A0"
	CharArrow    = "\u2012\u25b6"
	CharVertLine = "\u2502"
)

var (
	regLicenseValid   = regexp.MustCompile(`:\s*(\d+)\s*`)
	regLicenseInvalid = regexp.MustCompile(`:\s*(\d+)\s*\(.*\)`)
)

type Player struct {
	LastName     string `json:"lastname"`
	FirstName    string `json:"firstname"`
	License      string `json:"license"`
	LicenseValid bool   `json:"license_valid"`
	Ranking      string `json:"ranking"`
	Club         string `json:"club"`
}

func cleanText(s string) string {
	return strings.ReplaceAll(s, "\u00a0", "")
}

func extractLicense(s string) (res string, licValid bool) {
	s = cleanText(s)

	m := regLicenseInvalid.FindStringSubmatch(s)
	if len(m) > 1 {
		res = m[1]
		licValid = false
		return
	}

	m = regLicenseValid.FindStringSubmatch(s)
	if len(m) > 1 {
		res = m[1]
		licValid = true
		return
	}

	return
}

func scrapPageSinglePlayer(url string) {
	c := colly.NewCollector(
		colly.UserAgent("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/81.0.4044.138 Safari/537.36"),
		colly.IgnoreRobotsTxt(),
		colly.AllowURLRevisit(),
		//colly.Debugger(&debug.LogDebugger{}),
		colly.MaxBodySize(0),
	)

	p := &Player{}

	c.OnHTML("table table table td.titlight, table table table td.titrepage", func(h *colly.HTMLElement) {
		name := cleanText(h.Text)
		str := strings.Split(name, ",")
		if len(str) == 2 {
			p.LastName = strings.TrimSpace(str[0])
			p.FirstName = strings.TrimSpace(str[1])
		}
	})

	c.OnHTML("td:contains('No License') + td", func(h *colly.HTMLElement) {
		s := cleanText(h.Text)

		if s != "" && p.License == "" {
			lic, valid := extractLicense(s)
			p.License = lic
			p.LicenseValid = valid
		}
	})

	c.OnHTML("td:contains('Classement') + td", func(h *colly.HTMLElement) {
		s := cleanText(h.Text)

		if s != "" && p.Ranking == "" {
			m := regLicenseValid.FindStringSubmatch(s)
			if len(m) > 1 {
				p.Ranking = m[1]
			}

			s = strings.ReplaceAll(s, " ", "")
			if s == ":NC" {
				p.Ranking = "NC"
			}
		}
	})

	c.OnHTML("td:contains('Club') + td a", func(h *colly.HTMLElement) {
		if h.Text != "" && p.Club == "" {
			p.Club = h.Text
		}
	})

	c.Visit(url)

	//fmt.Printf("Player: %s %s License:%s  Classement:%s  Club:%s  Valid:%v\n", p.FirstName, p.LastName, p.License, p.Ranking, p.Club, p.LicenseValid)
	j, err := json.Marshal(p)
	if err != nil {
		fmt.Printf(`{"error": true, "message": "failed to marshal data for player %s/%s. %v"}`, p.FirstName, p.LastName, err)
		fmt.Printf("\n")
	} else {
		fmt.Println(string(j))
	}
}

func scrapPageAlpha(letter string) {
	url := fmt.Sprintf(PLAYER_LIST_URL, letter)

	c := colly.NewCollector(
		colly.UserAgent("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/81.0.4044.138 Safari/537.36"),
		colly.IgnoreRobotsTxt(),
		colly.AllowURLRevisit(),
		//colly.Debugger(&debug.LogDebugger{}),
		colly.MaxBodySize(0),
	)

	c.OnHTML("a", func(h *colly.HTMLElement) {
		href := h.Attr("href")
		if strings.HasPrefix(href, "joueurs.php") {
			link := fmt.Sprintf("%s/%s", BASE_URL, href)
			scrapPageSinglePlayer(link)
		}
	})

	c.Visit(url)
}

func execApp() {
	letters := []string{"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"}
	fmt.Printf(`{"progress":0,"total":%d}`, len(letters))
	fmt.Printf("\n")

	for i, l := range letters {
		scrapPageAlpha(l)

		//Print progress
		fmt.Printf(`{"progress":%d,"total":%d}`, i+1, len(letters))
		fmt.Printf("\n")
	}
}

func main() {
	execApp()
}
