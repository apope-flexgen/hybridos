import { AppThrottleConfig } from './config-fields/throttle/AppThrottleConfig';

// AppConfig consts
export const APP_THROTTLE_CONFIG = new AppThrottleConfig(60, 210);

// Other application config constants
export const LOGIN_API_TTL = 60;
export const LOGIN_API_LIMIT = 5;
export const FIMS_API_TTL = 60;
export const FIMS_API_LIMIT = 210;
export const FIMS_WS_TTL = 60;
export const FIMS_WS_LIMIT = 210;
export const REST_API_TTL = 60;
export const REST_API_LIMIT = 620;
