/* This script takes the tamalib opcode table (input.txt) and reorganizes it by most-frequently-called function
 * to least-frequently-called function. The list of functions in call-frequency order (profiler-output.txt) was
 * created by taking the output from a gprof session with tamatool and removing all irrelevant information.
 * 
 * The resulting output (output.txt) contains the original opcode table sorted according to the profiler output.
 * Any functions which were not included in the profiler output were appended to the end of the opcode table.
 */

string OUTPUT_TEMPLATE = $"static const op_t ops[] = {{\r\n\r\n\t{0}\r\n\t{{NULL, 0, 0, 0, 0, 0, NULL}},\r\n}};\r\n";

//read files
List<string> inputLines = File.ReadAllLines("input.txt").ToList();
string[] orderedFuncs = File.ReadAllLines("profiler-output.txt");
List<string> outputLines = new List<string>();

//sort output by function call frequency
foreach (string currFuncName in orderedFuncs)
{
    string targetLine = inputLines.First(l => l.Contains(currFuncName));
    outputLines.Add(targetLine);
}

//add lines which were missing from profiler info to output
List<string> unprofiledLines = inputLines.Where(l => !outputLines.Contains(l)).ToList();

outputLines.AddRange(unprofiledLines);

//check for missing output
foreach (string outputLine in outputLines)
{
    if (!inputLines.Contains(outputLine))
    {
        throw new Exception($"Line '{outputLine}' missing from output");
    }
}

//construct C array def with output lines
string outputArrayText = "static const op_t ops[] = {" + System.Environment.NewLine + 
    string.Join(System.Environment.NewLine, outputLines) + System.Environment.NewLine + 
    "\t{NULL, 0, 0, 0, 0, 0, NULL}," + System.Environment.NewLine + 
    "};";

File.WriteAllText("output.txt", outputArrayText);
