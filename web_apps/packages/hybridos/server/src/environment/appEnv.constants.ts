import { AppThrottleConfig } from "./config-fields/throttle/AppThrottleConfig";

// AppConfig consts
export const APP_SERVER_PORT = 443;
export const MONGO_DB_NAME = "hybridos_authentication";
export const MONGO_URL = "mongodb://localhost:27017";
export const HTTP_TIMEOUT = 10000;
export const ACCESS_TOKEN_TIMEOUT = 180;
export const REFRESH_TOKEN_TIMEOUT = 28800;
export const JWT_SECRET_FIMS_SOCKET = "fims-web-socket-one-time-token";
export const ACCESS_TOKEN_SECRET_FIMS_SOCKET = "fims-web-socket-one-time-token";
export const APP_THROTTLE_CONFIG = new AppThrottleConfig(60, 210);

//TODO: move supersecretkey here

// Other application config constants
export const LOGIN_API_TTL = 60
export const LOGIN_API_LIMIT = 5
export const FIMS_API_TTL = 60
export const FIMS_API_LIMIT = 210
export const FIMS_WS_TTL = 60
export const FIMS_WS_LIMIT = 210
export const REST_API_TTL = 60
export const REST_API_LIMIT = 620