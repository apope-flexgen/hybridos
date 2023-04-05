import { IsNotEmpty, IsString } from 'class-validator'

export class EventParams {
    @IsNotEmpty()
    @IsString()
    siteId: string
}
