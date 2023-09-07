import { ApiProperty } from '@nestjs/swagger';
import { IsString, IsNotEmpty, IsOptional } from 'class-validator';
import { FIMS_BODY, FIMS_URI } from '../api.constants';
export class BodyDTO {
    @ApiProperty({ description: FIMS_URI, required: true})
    @IsNotEmpty()
    @IsString()
    uri: string;

    @ApiProperty({ description: FIMS_URI, required: true})
    @IsNotEmpty()
    @IsString()
    replyto: string;

    @ApiProperty({ description: FIMS_BODY})
    @IsOptional()
    body: string;
}