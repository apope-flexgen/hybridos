import { IsString, IsNotEmpty, IsOptional } from 'class-validator'

export class GetDTO {
    @IsNotEmpty()
    @IsString()
    uri: string
    @IsNotEmpty()
    @IsString()
    replyto: string
}

export class BodyDTO {
    @IsNotEmpty()
    @IsString()
    uri: string
    @IsNotEmpty()
    @IsString()
    replyto: string
    @IsOptional()
    body: string
}

export class FimsMsgDTO {
    @IsNotEmpty()
    @IsString()
    method: string
    @IsNotEmpty()
    @IsString()
    uri: string
    @IsNotEmpty()
    @IsString()
    replyto: string
    // body = '';
    body: any
    username = ''
}
