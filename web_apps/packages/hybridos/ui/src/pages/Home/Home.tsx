import LogoutButton from 'src/components/LogoutButton';
import { PageProps } from 'src/pages/PageTypes';

const Home = ({ pageName }: PageProps) => (
  <>
    <div>{pageName}</div>
    <LogoutButton />
  </>
);

export default Home;
