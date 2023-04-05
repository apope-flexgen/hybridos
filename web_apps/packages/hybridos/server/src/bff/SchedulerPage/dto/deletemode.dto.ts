import { IsString } from 'class-validator'
import { DeleteModeID } from '../../../../../shared/types/dtos/scheduler.dto'

export class DeleteMode {
    @IsString()
        id: DeleteModeID['id']
}
