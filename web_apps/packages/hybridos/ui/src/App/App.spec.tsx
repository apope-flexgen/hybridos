import { render } from '@testing-library/react';
import App from './App';

describe('renders learn react link', () => {
  it('sanity check', () => {
    const view = render(App());
    expect(view).toBeTruthy();
  });
});
