import { render } from '@testing-library/react'
import App from '../'

describe('renders learn react link', () => {
    it('sanity check', () => {
        const view = render(App())
        expect(view).toBeTruthy()
    })
    // render(<App />);
    // const linkElement = screen.getByText(/learn react/i);
    // expect(linkElement).toBeInTheDocument();
})
