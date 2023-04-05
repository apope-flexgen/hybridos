import { useContext } from 'react';
import { AuthContext } from 'src/contexts/AuthProvider';

export const useAuth = () => useContext(AuthContext);

export default useAuth;
