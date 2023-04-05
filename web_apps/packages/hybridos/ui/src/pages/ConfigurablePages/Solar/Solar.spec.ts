import { render } from '@testing-library/react';
import Solar from './Solar';

describe('<Solar/>', () => {
  it('sanity check', () => {
    const view = render(Solar());
    expect(view).toBeTruthy();
  });
});
