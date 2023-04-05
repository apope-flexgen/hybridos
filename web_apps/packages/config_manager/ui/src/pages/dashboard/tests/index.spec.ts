import { render } from '@testing-library/react'
import Dashboard from '..'

describe('<Dashboard/>', () => {
    it('sanity check', () => {
        const view = render(Dashboard({ pageName: 'Dashboard' }))
        expect(view).toBeTruthy()
    })
})
