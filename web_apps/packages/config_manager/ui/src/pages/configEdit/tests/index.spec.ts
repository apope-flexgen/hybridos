import { render } from '@testing-library/react'
import ConfigEdit from '../'

describe('<ConfigEdit/>', () => {
    it('sanity check', () => {
        // @ts-ignore
        const view = render(ConfigEdit({ pageName: 'Config Edit' }))
        expect(view).toBeTruthy()
    })
})
