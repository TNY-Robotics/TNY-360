// https://nuxt.com/docs/api/configuration/nuxt-config
export default defineNuxtConfig({
  compatibilityDate: '2025-07-15',
  devtools: { enabled: false },
  modules: ['@nuxt/ui'],
  css: [ '~/assets/main.css' ],
  ssr: false,
  sourcemap: {
    server: false,
    client: false,
  },
  experimental: {
    payloadExtraction: false,
  },
  nitro: {
    preset: 'static',
    compressPublicAssets: true,
    minify: true
  },
  app: {
    baseURL: '/'
  },
  vite: {
    build: {
      cssCodeSplit: false,
      chunkSizeWarningLimit: 2000,
      rollupOptions: {
        output: {
          inlineDynamicImports: true, // Only one js file, easier for embedded systems
        }
      }
    }
  }
})