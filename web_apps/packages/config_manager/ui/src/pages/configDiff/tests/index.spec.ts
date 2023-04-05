import { render } from '@testing-library/react'
import ConfigDiff from '../'

describe('<ConfigDiff/>', () => {
    it('sanity check', () => {
        // @ts-ignore
        const view = render(ConfigDiff({ pageName: 'Dashboard' }))
        expect(view).toBeTruthy()
    })
})
