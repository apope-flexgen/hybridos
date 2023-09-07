import { ApiProperty } from "@nestjs/swagger";
import { FIMS_BODY, FIMS_URI } from "../api.constants";

export class FimsMsgResponse {
    @ApiProperty({required: false})
	method: string;
    @ApiProperty({description: FIMS_URI, required: false})
	uri: string;
    @ApiProperty({description: FIMS_URI, required: false})
	replyto: string;
    @ApiProperty({description: FIMS_BODY, required: false})
	body: string;
    @ApiProperty({required: false})
	username: string;
}