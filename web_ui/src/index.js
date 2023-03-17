import React from 'react';
import ReactDOM from 'react-dom/client';
import App from './App';
import { Provider } from 'react-redux';
import { configureStore } from '@reduxjs/toolkit';
import { combineReducers } from 'redux';
import popupAlert from './reducers/popupAlert';
import multiUseModal from './reducers/multiUseModal';

//configureStore can only take one reducer as an argument, so you need to
//combine your reducers into one
const allReducers = combineReducers({
    popupAlert,
    multiUseModal
})

const store = configureStore({reducer: allReducers});
const container = document.getElementById('root');
const root = ReactDOM.createRoot(container);
root.render(
    <Provider store={store}>
        <App />
    </Provider>
);
