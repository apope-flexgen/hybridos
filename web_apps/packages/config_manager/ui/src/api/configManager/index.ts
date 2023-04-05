// Need to use the React-specific entry point to import createApi
import { createApi, fetchBaseQuery } from '@reduxjs/toolkit/query/react'

const baseUrl = 'http://localhost:3000/'
// Define a service using a base URL and expected endpoints
export const configManagerApiSlice = createApi({
    reducerPath: 'configManager',
    baseQuery: fetchBaseQuery({ baseUrl }),
    endpoints: (builder) => ({
        /*
         ** Here we define our endpoints that will auto-generate hooks
         ** that we can use in our component to extract the data from the API
         */
        getInitData: builder.query<any[], void>({ query: () => 'app' }),
        getConfigData: builder.query<any[], void>({ query: () => 'config/history' }),
    }),
})

// Export hooks for usage in functional components, which are
// auto-generated based on the defined endpoints
export const { useGetConfigDataQuery, useGetInitDataQuery } = configManagerApiSlice
