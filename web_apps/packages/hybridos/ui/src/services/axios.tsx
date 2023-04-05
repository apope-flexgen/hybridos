import axios from 'axios';
import SocketConnectionManager from './SocketConnectionManager';

export default axios.create();

export const axiosWebUIInstance = axios.create({
  headers: { 'Content-Type': 'application/json' },
  withCredentials: true,
});

export const axiosSocketConnectionManagerInstance = axios.create({
  transformResponse: async (data) => {
    // TODO: fix lint
    // eslint-disable-next-line no-useless-catch
    try {
      const transform = JSON.parse(data);
      if (transform?.message === 'jwt expired') {
        const refresh = await axios.get('/api/refresh_token', {
          withCredentials: true,
        });
        SocketConnectionManager.setAccessToken(refresh.data.accessToken);
      }
      return transform;
    } catch (e) {
      throw e;
    }
  },
});
