import axios from 'src/services/axios';
import { useAuth } from './useAuth';

const useRefreshToken = () => {
  const { setAuth } = useAuth();

  const refresh = async () => {
    const response = await axios.get('/api/refresh_token', {
      withCredentials: true,
    });
    setAuth((prev: any) => ({ ...prev, accessToken: response.data.accessToken }));
    return response.data.accessToken;
  };
  return refresh;
};

export default useRefreshToken;
