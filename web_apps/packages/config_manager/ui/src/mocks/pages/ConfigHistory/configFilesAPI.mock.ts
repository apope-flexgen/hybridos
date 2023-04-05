import { rest } from 'msw'
import configFilesMockData from './configFilesMockData.json'

export const configFilesMockedApi = rest.get('/configFiles', (req, res, ctx) => {
    return res(ctx.json(configFilesMockData))
})
