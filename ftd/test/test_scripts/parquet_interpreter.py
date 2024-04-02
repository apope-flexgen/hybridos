import os
import io
import gzip
import pandas as pd
import pyarrow.parquet as pq

def read_parquet_gz(file_path):
    with gzip.open(file_path, 'rb') as f:
        table = pq.read_table(f)
    return table.to_pandas()

def print_metadata(file_path):
    with gzip.open(file_path, 'rb') as f:
        parquet_file = pq.ParquetFile(f)

        metadata = parquet_file.metadata
        user_defined_metadata = metadata.metadata

        print("Metadata:")
        for key, value in user_defined_metadata.items():
            print(f'  "{key.decode()}" : "{value.decode()}"')

def process_parquet_gz_files(folder_path):
    files = os.listdir(folder_path)
    
    parquet_gz_files = [file for file in files if file.endswith('.parquet.gz')]
    print(parquet_gz_files)
    
    for file in parquet_gz_files:
        file_path = os.path.join(folder_path, file)
        try:
            df = read_parquet_gz(file_path)
            print(f"{file} ({os.path.getsize(file_path)} bytes):")
            print(f"{len(df.columns)} columns:\n  {df.columns.tolist()}")
            print_metadata(file_path)
        except FileNotFoundError:
            print(f"File '{file}' not found.")
        except Exception as e:
            print(f"An error occurred while processing {file}: {e}")
        print("----------------------------------------------")

folder_path = '/home/vagrant/ftd_test/output'

if os.path.exists(folder_path) and os.path.isdir(folder_path):
    process_parquet_gz_files(folder_path)
else:
    print(f"Folder '{folder_path}' not found.")
