import { render } from '@testing-library/react';
import Scheduler from './Scheduler';

describe('<Scheduler/>', () => {
  it('sanity check', () => {
    // TODO: fix lint
    // eslint-disable-next-line @typescript-eslint/ban-ts-comment
    // @ts-ignore
    const view = render(Scheduler({ pageName: 'Scheduler' }));
    expect(view).toBeTruthy();
  });
});
