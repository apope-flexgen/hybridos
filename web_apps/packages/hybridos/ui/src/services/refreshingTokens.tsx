/* eslint-disable import/no-mutable-exports */
/* eslint-disable no-return-assign */
/* eslint-disable @typescript-eslint/no-explicit-any */
// Global variable that stores Promise for request to /api/refresh_tokens that is currently in progress.
export let refreshingTokens: Promise<any> | undefined;

export const setRefreshingTokens = (fn: Promise<any> | undefined) => (refreshingTokens = fn);
export const isRefreshingTokens = () => !(refreshingTokens === undefined);
