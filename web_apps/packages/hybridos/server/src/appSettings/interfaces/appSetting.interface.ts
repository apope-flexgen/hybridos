export interface AppSetting {
    password: PasswordSetting
    radius: RadiusSetting
}

export interface PasswordSetting {
    multi_factor_authentication: boolean
    password_expiration: boolean
    minimum_password_length: number
    maximum_password_length: number
    password_expiration_interval: string
    old_passwords: number
    password_regular_expression: string

    lowercase: boolean
    uppercase: boolean
    digit: boolean
    special: boolean
}

export interface RadiusSetting {
    is_enabled: boolean
    ip_address: string
    port: string
    secret_phrase: string
    wait_time: number
    is_local_auth_disabled: boolean
}
