
from test_matrix import *

#---------------------------------------------------------------------------

# https://htmlcolorcodes.com/color-names/
Template = r"""\documentclass[multi={mymath},border=2pt]{standalone}
\usepackage{xcolor}
\definecolor{gainsboro}{RGB}{220,220,220}
\definecolor{silver}{RGB}{192,192,192}
\usepackage{amsmath}
\newenvironment{mymath}{$\displaystyle}{$}
\begin{document}
\begin{mymath}
<CONTENT>
\end{mymath}
\end{document}
""".encode("ascii")

import tempfile
import subprocess
import os

def latex_to_pdf(expression, file_name):
    print(expression)
    tex_file = tempfile.NamedTemporaryFile(suffix=".tex", delete=False)
    tex_file.write(Template.replace(b"<CONTENT>", expression.encode("utf-8")))
    tex_file.close()
    dirname = os.path.dirname(file_name)
    if dirname == "":
        dirname = "."
    cmd = ["pdflatex", "-output-directory=%s" % dirname,
           "-jobname=%s" % file_name, tex_file.name]
    print(cmd)
    try:
        subprocess.check_call(cmd)
    except:
        os.remove(tex_file.name)
        raise
    os.remove(tex_file.name)
    os.remove(file_name+".aux")
    os.remove(file_name+".log")

#--------------------------------------------------
    
latex_to_pdf(str_system, "ex-system")
latex_to_pdf(latex_matrix, "ex-matrix")    
latex_to_pdf(latex_rref_matrix, "ex-rref")
latex_to_pdf(latex_rref_lt_matrix, "ex-rref-lt")
    
#---------------------------------------------------------------------------


