import subprocess
from threading import Thread
import time


# Steps to run once before any test execution
def before_all(context):
    context.fims_already_running = False
    # Start fims_server if it's not already running
    try:
        subprocess.check_output(["pgrep", "fims_server"])
        context.fims_already_running = True
        # If no exception then it's already running
    except subprocess.CalledProcessError:
        # Not running, start fims_server manually
        subprocess.Popen(["fims_server"])


# Steps to run once after all tests have executed
def after_all(context):
    # Stop fims_server if it wasn't running when we started
    if not context.fims_already_running:
        subprocess.run(["pkill", "fims_server"])


def before_feature(context, feature):
    # Start fims_server if it's not already running
    try:
        subprocess.check_output(["pgrep", "fims_server"])
        # If no exception then it's already running
    except subprocess.CalledProcessError:
        # Not running, start fims_server manually
        subprocess.Popen(["fims_server"])

    full_path = feature.filename.split("/")
    if len(full_path) > 1:
        sub_dir = "/".join(full_path[1:-1]) + "/"
    else:
        sub_dir = ""
    context.threads = []
    context.threads.append(Thread(target=subprocess.run, kwargs={"args": [
                           "go_metrics", f"configs/{sub_dir}{feature.name}.json"], "stdout": subprocess.PIPE, "stderr": subprocess.PIPE, "check": False}))
    for thread in context.threads:
        thread.start()
    time.sleep(1)


def after_feature(context, feature):
    subprocess.run(["pkill", "go_metrics"], stdout=subprocess.PIPE,
                   stderr=subprocess.PIPE, check=False)
    if hasattr(context, "threads"):
        for thread in context.threads:
            thread.join()


def after_scenario(context, scenario):
    subprocess.run(["pkill", "fims_listen"], stdout=subprocess.PIPE,
                   stderr=subprocess.PIPE, check=False)
    if hasattr(context, "fims_listen_threads"):
        for thread in context.fims_listen_threads:
            thread.join()
