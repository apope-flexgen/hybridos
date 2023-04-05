import { Allow, IsString } from 'class-validator'
import { ModeDTO } from '../../../../../shared/types/dtos/scheduler.dto'

export class Modes {
    [id: ModeDTO['id']] : {
        mode: ModeDTO['mode']
    }        
}
