import React from 'react';
import ReactDOM from 'react-dom/client';
import './index.css';
import ErrorProvider from 'src/contexts/ErrorContext';
import App from './App/App';
import { AuthProvider } from './contexts/AuthProvider';

// TODO: comment this out to not use mocked data
// if (process.env.NODE_ENV === 'development') {
//     const { worker } = require('./mocks/browser')
//     worker.start()
// }

const root = ReactDOM.createRoot(document.getElementById('root') as HTMLElement);
root.render(
  <React.StrictMode>
    <AuthProvider>
      <ErrorProvider>
        <App />
      </ErrorProvider>
    </AuthProvider>
  </React.StrictMode>,
);
