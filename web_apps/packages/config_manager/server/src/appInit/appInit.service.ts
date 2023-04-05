import { Injectable } from '@nestjs/common'

// TODO: move this into a config file once environment variables are added to project
const mockData = {
    timezone: 'America/Chicago',
    configDiff: true,
    configEdit: true,
    configHistory: true,
    siteServerSelection: true,
}

@Injectable()
export class AppInitService {
    getSystemConfiguration = () => {
        return mockData
    }
}
