import { createContext, useMemo, useState } from 'react';

type UserAuth = {
  accessToken?: string | null
};

export type AuthContextType = {
  auth: UserAuth | null,
  setAuth: React.Dispatch<React.SetStateAction<UserAuth>>,
};

export const AuthContext = createContext<AuthContextType>({
  auth: null,
  setAuth: () => {},
});

export const AuthProvider = ({ children }: any) => {
  const [auth, setAuth] = useState<UserAuth>({});

  const contextValue = useMemo(() => ({ auth, setAuth }), [auth]);

  return (
    <AuthContext.Provider value={contextValue}>
      {children}
    </AuthContext.Provider>
  );
};
