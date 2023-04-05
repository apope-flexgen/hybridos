import { IsInt, IsIP, IsPort, IsString, Max, Min } from 'class-validator'

export class RadiusTestDto {
    @IsIP()
    ip_address: string
    @IsPort()
    port: string
    @IsString()
    secret_phrase: string
    @IsInt()
    @Min(0)
    @Max(30000)
    wait_time: number
    @IsString()
    username: string
    @IsString()
    password: string
}
