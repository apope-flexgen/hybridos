import { render } from '@testing-library/react'
import Home from '../'

describe('<Home/>', () => {
    it('sanity check', () => {
        const view = render(Home({ pageName: 'Home' }))
        expect(view).toBeTruthy()
    })
})
