import { ApiProperty } from "@nestjs/swagger"
import { IsNotEmpty, IsString } from "class-validator"
import { FimsDescriptions } from "../fims.constants"

export class FimsMsg {
    @ApiProperty({ description: FimsDescriptions.method })
    @IsString()
    method: string
    @ApiProperty({ description: FimsDescriptions.bodyURI })
    @IsString()
    uri: string
    @ApiProperty({ description: FimsDescriptions.bodyReplyTo })
    @IsString()
    replyto: string
    @ApiProperty({ description: FimsDescriptions.body })
    @IsNotEmpty()
    body: any // TODO: Fix this type. Was (string | Record<string, unknown>), but error'd out.
    @ApiProperty({ description: FimsDescriptions.username })
    @IsString()
    username: string
}
