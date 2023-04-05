// TODO: fix lint
/* eslint-disable no-param-reassign */
import { InternalAxiosRequestConfig } from 'axios';
import { useCallback, useEffect } from 'react';
import { useErrorContext } from 'src/contexts/ErrorContext';
import { ErrorContextType } from 'src/contexts/ErrorContext/types';
import SocketConnectionManager from 'src/services/SocketConnectionManager';
import { axiosWebUIInstance } from 'src/services/axios';
import { useAuth } from './useAuth';
import useRefreshToken from './useRefresh';

const useAxiosWebUIInstance = (skipGenericError = false) => {
  const refresh = useRefreshToken();
  const { auth } = useAuth();
  const { showErrorModal } = useErrorContext() as ErrorContextType;

  const appendApiPath = (config: InternalAxiosRequestConfig<any>) => {
    if (config?.url && !config.url.startsWith('/api')) {
      config.url = `/api${config.url}`;
    }
  };

  const appendAuthHeader = useCallback((config: InternalAxiosRequestConfig<any>) => {
    if (!config.headers!.Authorization) {
      config.headers!.Authorization = auth?.accessToken;
    }
  }, [auth?.accessToken]);

  useEffect(() => {
    const requestIntercept = axiosWebUIInstance.interceptors.request.use(
      (config) => {
        appendAuthHeader(config);
        appendApiPath(config);
        return config;
      },
      (error) => Promise.reject(error),
    );

    const responseIntercept = axiosWebUIInstance.interceptors.response.use(
      (response) => response,
      async (error) => {
        const prevRequest = error?.config;
        const errorMessage = (error?.response?.data?.message || '').toString().toLowerCase();
        const jwtError = (
          errorMessage === 'jwt expired'
                    || errorMessage === 'jwt malformed'
                    || errorMessage === 'no auth token'

        );
        if (jwtError && !prevRequest?.sent) {
          prevRequest.sent = true;
          const accessToken = await refresh();
          prevRequest.headers.Authorization = `${accessToken}`;
          SocketConnectionManager.setAccessToken(accessToken);
          return axiosWebUIInstance(prevRequest);
        }
        if (!skipGenericError) {
          const { response: { data, status, statusText } } = error;
          showErrorModal({
            title: statusText,
            description: data.message || data,
            errorCode: status,
          });
        }

        return Promise.reject(error);
      },
    );

    return () => {
      axiosWebUIInstance.interceptors.request.eject(requestIntercept);
      axiosWebUIInstance.interceptors.response.eject(responseIntercept);
    };
  }, [appendAuthHeader, refresh, showErrorModal, skipGenericError]);

  return axiosWebUIInstance;
};

export default useAxiosWebUIInstance;
