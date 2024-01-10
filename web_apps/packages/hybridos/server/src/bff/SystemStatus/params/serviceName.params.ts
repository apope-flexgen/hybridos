import { IsNotEmpty, IsString } from "class-validator";
import { systemStatusDescriptions } from "../systemStatus.constants";
import { ApiProperty } from "@nestjs/swagger";

export class ServiceName {
    @ApiProperty({ description: systemStatusDescriptions.serviceName })
    @IsNotEmpty()
    @IsString()
    serviceName: string
}