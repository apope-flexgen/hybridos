import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'
import viteTsconfigPaths from 'vite-tsconfig-paths'
import svgrPlugin from 'vite-plugin-svgr'

const DEBUG_LOGS = true

// https://vitejs.dev/config/
export default defineConfig({
    plugins: [
        react({
            include: '**/*.{jsx,tsx}',
        }),
        viteTsconfigPaths(),
        svgrPlugin(),
    ],
    build: {
        outDir: './build',
    },
    server: {
        port: 3000,
        proxy: {
            '/api': {
                target: 'https://172.16.1.80:3001',
                changeOrigin: true,
                secure: false,
                configure: (proxy, _options) => {
                    proxy.on('error', (err, _req, _res) => {
                        DEBUG_LOGS && console.log('proxy error', err)
                    })
                    proxy.on('proxyReq', (proxyReq, req, _res) => {
                        DEBUG_LOGS &&
                            console.log('Sending Request to the Target:', req.method, req.url)
                    })
                    proxy.on('proxyRes', (proxyRes, req, _res) => {
                        DEBUG_LOGS &&
                            console.log(
                                'Received Response from the Target:',
                                proxyRes.statusCode,
                                req.url
                            )
                    })
                },
            },
        },
        cors: true,
    },
    preview: {
        port: 3000,
        proxy: {
            '/api': {
                target: 'https://172.16.1.80:3001',
                changeOrigin: true,
                secure: false,
                configure: (proxy, _options) => {
                    proxy.on('error', (err, _req, _res) => {
                        DEBUG_LOGS && console.log('proxy error', err)
                    })
                    proxy.on('proxyReq', (proxyReq, req, _res) => {
                        DEBUG_LOGS &&
                            console.log('Sending Request to the Target:', req.method, req.url)
                    })
                    proxy.on('proxyRes', (proxyRes, req, _res) => {
                        DEBUG_LOGS &&
                            console.log(
                                'Received Response from the Target:',
                                proxyRes.statusCode,
                                req.url
                            )
                    })
                },
            },
        },
        cors: true,
    },
})
