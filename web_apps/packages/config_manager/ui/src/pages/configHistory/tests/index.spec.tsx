import { server } from 'src/mocks/api/server'
import { rest } from 'msw'
import { screen } from '@testing-library/react'
import { renderWithProviders } from '../../../components/testUtilities/renderWithProviders'
import ConfigHistoryView from '../'

describe('<ConfigHistory /> component', () => {
    it('handles rendering properly', async () => {
        renderWithProviders(<ConfigHistoryView pageName='Config History' />)

        screen.getByText('Loading...')
    })

    it('handles error response', async () => {
        // force msw to return error response
        server.use(
            rest.get('http://localhost:3000/configFiles', (req, res, ctx) => {
                return res(ctx.status(500))
            })
        )

        renderWithProviders(<ConfigHistoryView pageName='Config History' />)

        screen.getByText('Loading...')

        await screen.findByText('Oh no, there was an error')
    })
})
