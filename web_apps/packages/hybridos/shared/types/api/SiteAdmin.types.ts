export interface SiteAdmins {
    _id: string
    password: PasswordSettings
    radius: RadiusSettings
    __v: number
}

export interface PasswordSettings {
  _id: string;
  multi_factor_authentication: boolean;
  password_expiration: boolean;
  minimum_password_length: number;
  maximum_password_length: number;
  password_expiration_interval: string;
  old_passwords: number;
  lowercase: boolean;
  uppercase: boolean;
  digit: boolean;
  special: boolean;
}

export interface RadiusSettings {
  _id: string;
  port: string;
  is_enabled: boolean;
  ip_address: string;
  secret_phrase: string;
  wait_time: number;
  is_local_auth_disabled: boolean;
}

export interface RadiusTestSettings {
  port: string;
  ip_address: string;
  secret_phrase: string;
  wait_time: number;
  username: string;
  password: string;
}
