import subprocess, sys
import unittest


path = f"{sys.argv[1]}"


def extract_output(output: str):
    start_index = output.find("Tree:")
    end_index = output[start_index:].find(".")
    return output[start_index:start_index + end_index + 1].strip()


class TestOutput(unittest.TestCase):
    def test_allows_printing_structure_3_leaf_btree(self):
        global path
        
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
database> database> Need to implement searching an internal node."""

        
        process = subprocess.run(
            [path],
	        input=input,
	        text=True,
	        capture_output=True
        )

        self.assertEqual(expected.strip(), extract_output(process.stdout))

    

if __name__ == "__main__":
    unittest.main(argv=[""], exit=False)
