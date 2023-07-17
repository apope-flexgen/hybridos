// TODO: fix lint
/* eslint-disable no-param-reassign */
import axios, { InternalAxiosRequestConfig } from 'axios';
import { useCallback, useEffect } from 'react';
import { useErrorContext } from 'src/contexts/ErrorContext';
import { ErrorContextType } from 'src/contexts/ErrorContext/types';
import RealTimeService from 'src/services/RealTimeService/realtime.service';
import { axiosWebUIInstance } from 'src/services/axios';
import {
  setRefreshingTokens,
  isRefreshingTokens,
  refreshingTokens,
} from 'src/services/refreshingTokens';
import { useAuth } from './useAuth';

const useAxiosWebUIInstance = (skipGenericError?: boolean) => {
  const { auth } = useAuth();
  const { showErrorModal } = useErrorContext() as ErrorContextType;

  const appendApiPath = (config: InternalAxiosRequestConfig<any>) => {
    if (config?.url && !config.url.startsWith('/api')) {
      config.url = `/api${config.url}`;
    }
  };

  const appendAuthHeader = useCallback(
    async (config: InternalAxiosRequestConfig<any>) => {
      if (isRefreshingTokens()) {
        const response = await refreshingTokens;
        config.headers.Authorization = response.data.accessToken;
      } else if (!config.headers.Authorization) {
        config.headers.Authorization = auth?.accessToken;
      }
    },
    [auth?.accessToken],
  );

  useEffect(() => {
    const requestIntercept = axiosWebUIInstance.interceptors.request.use(
      async (config) => {
        await appendAuthHeader(config);
        appendApiPath(config);
        return config;
      },
      (error) => Promise.reject(error),
    );

    const responseIntercept = axiosWebUIInstance.interceptors.response.use(
      (response) => response,
      async (error) => {
        try {
          const realTimeService = RealTimeService.Instance;
          const prevRequest = error?.config;
          const errorMessage = (error?.response?.data?.message || '').toString().toLowerCase();
          const jwtError = errorMessage === 'jwt expired'
            || errorMessage === 'jwt malformed'
            || errorMessage === 'no auth token';

          if (!isRefreshingTokens()) {
            setRefreshingTokens(
              axios.get('/api/refresh_token', {
                withCredentials: true,
              }),
            );
          }
          if (jwtError && !prevRequest?.sent) {
            prevRequest.sent = true;
            const response = await refreshingTokens;
            const { accessToken } = response.data;
            auth!.accessToken = accessToken;
            prevRequest.headers.Authorization = `${accessToken}`;
            realTimeService.setAccessToken(accessToken);
            return await axiosWebUIInstance(prevRequest);
          }
          if (!skipGenericError) {
            const {
              response: { data, status, statusText },
            } = error;
            showErrorModal({
              title: statusText,
              description: data.message || data,
              errorCode: status,
            });
          }
        } finally {
          setRefreshingTokens(undefined);
        }
        return Promise.reject(error);
      },
    );

    return () => {
      axiosWebUIInstance.interceptors.request.eject(requestIntercept);
      axiosWebUIInstance.interceptors.response.eject(responseIntercept);
    };
  }, [appendAuthHeader, auth, showErrorModal, skipGenericError]);

  return axiosWebUIInstance;
};

export default useAxiosWebUIInstance;
