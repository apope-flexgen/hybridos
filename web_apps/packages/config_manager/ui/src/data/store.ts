import { combineReducers, configureStore } from '@reduxjs/toolkit'
import { configManagerApiSlice } from '../api/configManager'
import type { PreloadedState } from '@reduxjs/toolkit'
import { setupListeners } from '@reduxjs/toolkit/dist/query'

/* We save our app's reducers for every slice here. */
const rootReducer = combineReducers({
    [configManagerApiSlice.reducerPath]: configManagerApiSlice.reducer,
    // NOTE: can add more reducers here
})

/*
 * Our app's store configuration goes here.
 * Read more about Redux's configureStore in the docs:
 * https://redux-toolkit.js.org/api/configureStore
 */
export const setupStore = (preloadedState?: PreloadedState<RootState>) =>
    configureStore({
        reducer: rootReducer,
        middleware: (getDefaultMiddleware) =>
            // adding the api middleware enables caching, invalidation, polling and other features of `rtk-query`
            // Note: can concat more middlewares here
            getDefaultMiddleware().concat(configManagerApiSlice.middleware),
        preloadedState,
    })

setupListeners(setupStore({}).dispatch)

export type RootState = ReturnType<typeof rootReducer>
export type AppStore = ReturnType<typeof setupStore>
export type AppDispatch = AppStore['dispatch']
