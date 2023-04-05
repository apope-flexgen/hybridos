import { IsString } from 'class-validator'
import { DeleteEventRequestParams } from '../../../../../shared/types/dtos/scheduler.dto'

export class DeleteEvent {
    @IsString()
    id: DeleteEventRequestParams['id']
}
