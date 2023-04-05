import { IsNotEmpty } from 'class-validator'

export class ReadUserParams {
    @IsNotEmpty()
    id: string
}
