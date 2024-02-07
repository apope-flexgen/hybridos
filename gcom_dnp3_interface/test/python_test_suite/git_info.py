'''
Import git information about the current branch of this repo.
'''
import subprocess

class GitInfo:
    '''
    Initializing a GitInfo object will import git information about the current
    branch of this repo.
    '''
    def __init__(self):
        try:
            self.branch = subprocess.check_output(
                ["git", "rev-parse", "--abbrev-ref", "HEAD"]).strip().decode('utf-8')
            self.commit_hash = subprocess.check_output(
                ["git", "rev-parse", "HEAD"]).strip().decode('utf-8')
            self.author = subprocess.check_output(
                ["git", "log", "-1", "--pretty=format:'%an'"]).strip().decode('utf-8')
        except subprocess.CalledProcessError as called_process_error:
            print(f"An error occurred while fetching the Git info: {str(called_process_error)}")
            self.branch = "COULD NOT FETCH GIT INFO"
            self.commit_hash = "COULD NOT FETCH GIT INFO"
            self.author = "COULD NOT FETCH GIT INFO"


    def __str__(self):
        output_str = ""
        output_str += f"Branch: {self.branch}\n"
        output_str += f"Latest Commit Hash: {self.commit_hash}\n"
        output_str += f"Latest Commit Author: {self.author}\n"
        return output_str

    def get_info(self):
        '''
        Return a dictionary of the git information for this branch
        '''
        return {"git_branch":self.branch,
                "git_commit_hash":self.commit_hash,
                "git_commit_author":self.author}

git_info = GitInfo()
