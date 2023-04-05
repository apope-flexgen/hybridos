// TODO: fix lint
/* eslint-disable class-methods-use-this */
/* eslint-disable @typescript-eslint/no-this-alias */
/* eslint-disable no-constructor-return */
// FIXME: error handling

let instance: HttpService | null = null;

class HttpService {
  constructor() {
    if (instance) {
      return instance;
    }
    instance = this;
    return this;
  }

  get: (url: string) => Promise<any> = async (url: string) => {
    const response = await fetch(url);
    return response.json();
  };

  post: (url: string, body: any) => Promise<any> = async (url: string, body: any) => {
    const response = await fetch(url, {
      method: 'POST',
      body: JSON.stringify(body),
      headers: {
        'Content-Type': 'application/json',
      },
    });
    return response.json();
  };

  put: (url: string, body: any) => Promise<any> = async (url: string, body: any) => {
    const response = await fetch(url, {
      method: 'PUT',
      body: JSON.stringify(body),
      headers: {
        'Content-Type': 'application/json',
      },
    });
    return response.json();
  };

  delete: (url: string) => Promise<any> = async (url: string) => {
    const response = await fetch(url, {
      method: 'DELETE',
    });
    return response.json();
  };
}

export default new HttpService();
