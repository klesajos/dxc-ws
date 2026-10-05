Trimmed excerpt of the real `src/` call chain (main -> Game::run ->
Input::next -> toDirection -> Board::move). Eval runs are hermetic: Claude
can't read the repo, so `setup.sh` copies these files into the run's empty
workspace as `src/`. Keep them in sync by hand if the real call chain changes.
