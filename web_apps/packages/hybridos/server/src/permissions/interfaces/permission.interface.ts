export interface Permissions {
    [key: string]: Role
}

export interface Role {
    included: string
    [key: string]: Permission | string
}

export interface Permission {
    _accessLevel: number
    [key: string]: Permission | number
}
