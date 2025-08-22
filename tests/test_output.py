import os
import sys
import subprocess
import unittest
import tempfile


path = f"{sys.argv[1]}"


class TestOutput(unittest.TestCase):
    def test_allows_printing_structure_3_leaf_btree(self):
        def extract_output(output: str):
            start_index = output.find("Tree:")
            end_index = output[start_index:].find(".")
            return output[start_index:start_index + end_index + 1].strip()
    
        
        input = ''.join([f"insert {i} user{i} user{i}@example.com\n" for i in range(1, 15)])
        input += """
.btree\n
insert 15 user15 user15@example.com\n
.exit\n"""

        expected = """
Tree:
- internal (size 1)
  - leaf (size 7)
    - 1
    - 2
    - 3
    - 4
    - 5
    - 6
    - 7
  - key 7
  - leaf (size 7)
    - 8
    - 9
    - 10
    - 11
    - 12
    - 13
    - 14
database> database> Executed."""

        with tempfile.NamedTemporaryFile(mode="w+b") as tmp:
            temp_file_path = tmp.name    

        process = subprocess.run(
            [path, temp_file_path],
	        input=input,
	        text=True,
	        capture_output=True
        )
        
        os.remove(temp_file_path)
        self.assertEqual(expected.strip(), extract_output(process.stdout))

    def test_allows_inserting_max_rows(self):
        def extract_output(output: str):
            start_index = output.rfind("database> Executed.")
            return output[start_index:].strip()
            
        input = "".join([f"insert {i} user{i} user{i}@example.com\n" for i in range(1, 1401)])

        expected = """
database> Executed.
database> Need to implement updating parent after spliting."""
        
        with tempfile.NamedTemporaryFile() as tmp:
            temp_file_path = tmp.name
        
        process = subprocess.run(
            [path, temp_file_path],
            input=input,
            text=True,
            capture_output=True
        )

        os.remove(temp_file_path)
        #print(extract_output(process.stdout))
        self.assertEqual(expected.strip(), extract_output(process.stdout))


if __name__ == "__main__":
    unittest.main(argv=[""], exit=False)
