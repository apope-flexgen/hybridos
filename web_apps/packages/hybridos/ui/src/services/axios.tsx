import axios from 'axios';
import RealTimeService from 'src/services/RealTimeService/realtime.service';
import { refreshingTokens, isRefreshingTokens, setRefreshingTokens } from './refreshingTokens';

export default axios.create();

export const axiosWebUIInstance = axios.create({
  headers: { 'Content-Type': 'application/json' },
  withCredentials: true,
});

export const axiosSocketConnectionManagerInstance = axios.create();

axiosSocketConnectionManagerInstance.interceptors.request.use(
  async (config) => {
    if (isRefreshingTokens()) {
      const response = await refreshingTokens;
      // eslint-disable-next-line no-param-reassign
      config.headers.Authorization = response.data.accessToken;
    }
    return config;
  },
  (error) => Promise.reject(error),
);

axiosSocketConnectionManagerInstance.interceptors.response.use(
  (response) => response,
  async (error) => {
    try {
      const realTimeService = RealTimeService.Instance;
      const prevRequest = error?.config;
      const errorMessage = (error?.response?.data?.message || '').toString().toLowerCase();
      const jwtError = errorMessage === 'jwt expired'
        || errorMessage === 'jwt malformed'
        || errorMessage === 'no auth token';

      if (jwtError) {
        if (!isRefreshingTokens()) {
          setRefreshingTokens(
            axios.get('/api/refresh_token', {
              withCredentials: true,
            }),
          );
        }
        if (!prevRequest?.sent) {
          prevRequest.sent = true;
          const response = await refreshingTokens;
          const { accessToken } = response.data;
          prevRequest.headers.Authorization = `${accessToken}`;
          realTimeService.setAccessToken(accessToken);
          return await axiosSocketConnectionManagerInstance(prevRequest);
        }
      }
    } finally {
      setRefreshingTokens(undefined);
    }
    return Promise.reject(error);
  },
);
