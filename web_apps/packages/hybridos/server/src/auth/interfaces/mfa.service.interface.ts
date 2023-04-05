import { User } from '../../../../shared/types/dtos/auth.dto'

export const MFA_SERVICE = 'MfaService'
export interface IMfaService {
    generateURL(user: User): string
    generateQRCode(user: User): Promise<string>
    mfaResponse(user: User)
    authenticate(secret: string, totp: string): boolean
    checkIfSiteMfaEnabled(user: User)
}
