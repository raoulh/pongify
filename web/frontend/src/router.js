import { createRouter, createWebHashHistory } from 'vue-router'
import HomePage from './views/HomePage.vue'
import SeriesListPage from './views/SeriesListPage.vue'
import SeriePage from './views/SeriePage.vue'
import PodiumListPage from './views/PodiumListPage.vue'
import PodiumPage from './views/PodiumPage.vue'
import TablesPage from './views/TablesPage.vue'
import SearchPage from './views/SearchPage.vue'

const routes = [
  { path: '/', name: 'home', component: HomePage },
  { path: '/series', name: 'series', component: SeriesListPage },
  { path: '/serie/:index', name: 'serie', component: SeriePage, props: true },
  { path: '/podium', name: 'podiumList', component: PodiumListPage },
  { path: '/podium/:index', name: 'podium', component: PodiumPage, props: true },
  { path: '/tables', name: 'tables', component: TablesPage },
  { path: '/search', name: 'search', component: SearchPage },
]

export function createAppRouter() {
  return createRouter({
    history: createWebHashHistory(),
    routes
  })
}
