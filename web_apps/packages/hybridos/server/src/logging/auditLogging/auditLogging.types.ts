export interface AuditLog {
    modified_field: string
    modified_value: string | boolean
    username: string
    userrole: string
    created: number
    [key: string]: any
}
