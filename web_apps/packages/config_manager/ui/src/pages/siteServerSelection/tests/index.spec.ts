import { render } from '@testing-library/react'
import SiteServerSelection from '../'

describe('<SiteServerSelection/>', () => {
    it('sanity check', () => {
        // @ts-ignore
        const view = render(SiteServerSelection({ pageName: 'Site Server Selection' }))
        expect(view).toBeTruthy()
    })
})
