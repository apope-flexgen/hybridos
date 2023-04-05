import { setupWorker, rest } from 'msw';
import handlers from './handlers';

const worker = setupWorker(...handlers);

window.msw = {
  worker,
  rest,
};

export default worker;
