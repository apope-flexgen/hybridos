import { User } from '../../../../shared/types/dtos/auth.dto'

export const PASS_EXP_SERVICE = 'PassExpService'
export interface IPassExpService {
    parseInterval(interval: string): number
    passwordExpired(updatedDate: Date, timeInterval: string): boolean
    passExpResponse(user: User)
    checkIfPasswordExpired(user: User)
}
