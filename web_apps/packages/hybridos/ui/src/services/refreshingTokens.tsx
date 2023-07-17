// Global variable that stores Promise for request to /api/refresh_tokens that is currently in progress.
export let refreshingTokens: Promise<any> | undefined = undefined

export const setRefreshingTokens = (fn: Promise<any> | undefined) => refreshingTokens = fn
export const isRefreshingTokens = () => !(refreshingTokens === undefined)
