import { ConfigFileMetadata } from '../interfaces/configFileMetadata.interface'
import { ControllerVersionMetadata } from '../interfaces/controllerVersionMetadata.interface'

export class ConfigHistoryResponse {
    activeVersion: ControllerVersionMetadata
    files: ConfigFileMetadata[]
}
