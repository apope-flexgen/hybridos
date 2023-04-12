import { darkTheme, MuiButton } from '@flexgen/storybook';
import { useErrorContext } from 'src/contexts/ErrorContext';
import { ErrorContextType } from 'src/contexts/ErrorContext/types';
import useAuth from 'src/hooks/useAuth';
import useAxiosWebUIInstance from 'src/hooks/useAxios';
import { ThemeProvider } from 'styled-components';
import { useNavigate } from 'react-router-dom';

const LogoutButton = () => {
  const axiosInstance = useAxiosWebUIInstance(true);
  const { setAuth } = useAuth();
  const LOGOUT_URL = '/logout';
  const { showErrorModal, clearErrorModal } = useErrorContext() as ErrorContextType;
  const navigate = useNavigate();

  const logout = async () => {
    try {
      await axiosInstance.post(LOGOUT_URL, {});
      setAuth({});
      navigate('/');
      window.location.reload();
    } catch (e) {
      showErrorModal({
        title: 'Logout Failed',
        description: 'An error occurred while logging out.',
        iconType: 'error',
        extraPropsAndActions: {
          hasConfirmButton: true,
          confirmButtonLabel: 'Retry',
          confirmButtonAction: () => {
            logout();
            clearErrorModal();
          },
        },
      });
    }
  };

  return (
    <ThemeProvider theme={darkTheme}>
      <MuiButton
        label="LOG OUT"
        color="inherit"
        onClick={logout}
        sx={{ backgroundColor: 'transparent', ':hover': { backgroundColor: 'transparent' } }}
        variant="text"
      />
    </ThemeProvider>
  );
};

export default LogoutButton;
