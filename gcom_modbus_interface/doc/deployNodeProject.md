Packaging a Node.js script for deployment involves creating a package that includes the script itself, any necessary modules, and any other resources it needs to run. Below are steps you can follow to package your Node.js script into a tarball.

### Step-by-Step Guide

#### 1. **Create a Project Directory:**
   - If you haven’t already, place your script and any other necessary files into a new directory.
   ```bash
   mkdir my_project
   cd my_project
   ```

#### 2. **Initialize a Node.js Project:**
   - Create a `package.json` file by running `npm init`. This will guide you through creating a `package.json` file.
   ```bash
   npm init -y  # -y flag uses default values
   ```

#### 3. **Install Dependencies:**
   - Install any necessary packages and save them to the `package.json` file.
   ```bash
   npm install express body-parser request --save
   ```

#### 4. **Add Your Script:**
   - Place your Node.js script in the project directory.

#### 5. **Package the Project:**
   - Use `tar` to create a tarball of the project directory.
   ```bash
   cd ..
   tar -czvf my_project.tar.gz my_project
   ```

### Deployment Steps:

#### 1. **Transfer the Tarball:**
   - Send `my_project.tar.gz` to the remote system by using `scp`, `rsync`, or another method.

#### 2. **Extract the Tarball on the Remote System:**
   ```bash
   tar -xzvf my_project.tar.gz
   ```

#### 3. **Navigate to the Project Directory:**
   ```bash
   cd my_project
   ```

#### 4. **Install the Node Modules:**
   - If you didn’t include the `node_modules` directory in the tarball, install the packages with `npm install`.
   ```bash
   npm install
   ```

#### 5. **Run the Script:**
   ```bash
   node your_script.js <server_ip>
   ```

### Optional: Create a Start Script
- In your `package.json`, you can add a start script:

    ```json
    "scripts": {
      "start": "node your_script.js"
    }
    ```
- Then, on the remote system, you can start your app with `npm start`.

### Note:
- Ensure Node.js and npm are installed on the remote system.
- It's often better to exclude the `node_modules` directory from the tarball and run `npm install` on the remote system after extraction to install the necessary dependencies, as `node_modules` can contain platform-specific compiled binaries. To do this, you might want to create a `.gitignore` or `.npmignore` file to exclude `node_modules` from the tarball.