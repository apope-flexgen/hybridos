import {
    IsNotEmpty,
    IsNumber,
} from 'class-validator'

export class ThrottleData {
    @IsNumber()
    @IsNotEmpty()
    ttl: number

    @IsNumber()
    @IsNotEmpty()
    limit: number

    constructor(ttl: number, limit: number) {
        this.ttl = ttl;
        this.limit = limit;
    }
}
