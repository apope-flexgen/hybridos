import { rest } from 'msw'

const mockData = {
    timezone: 'America/Chicago',
    configDiff: true,
    configEdit: true,
    configHistory: true,
    siteServerSelection: true,
}

export const appInitMock = rest.get('/app', (req, res, ctx) => {
    const error = req.url.searchParams.get('error')

    // mocking error response for testing purposes
    if (error === 'network') {
        return res(
            ctx.status(400),
            ctx.json({
                errorMessage: 'Network error - Loading initial data',
            })
        )
    }

    // successful response
    return res(ctx.status(200), ctx.json(mockData))
})
